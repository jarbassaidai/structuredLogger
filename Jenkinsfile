pipeline {
  agent any
  stages {
    stage('Check Unix') {
      steps {
        isUnix()
      }
    }
    stage('cd temp') {
      steps {
        dir(path: '/tmp/work') {
          sh 'mkdir /tmp/work'
        }
        
      }
    }
    stage('fetch') {
      steps {
        sh '''date
'''
        git(url: 'https://github.com/jarbassaidai/structuredLogger', credentialsId: 'ea5106fd1e74aa4ccbe03bece63d8aadb755b003', branch: 'master')
      }
    }
    stage('complete') {
      steps {
        echo 'done'
      }
    }
  }
}